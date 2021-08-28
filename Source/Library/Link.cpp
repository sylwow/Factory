#pragma once

#include <algorithm>

#include "Link.hpp"
#include "Node.hpp"

namespace sd
{

    Random::Random() : _eng(_rd()), _distr(0, 1) {}

    Random &Random::get()
    {
        static Random r;
        return r;
    }

    double Random::nextDouble() { return _distr(_eng); }

    Link::Link(size_t id, double probability, std::shared_ptr<SourceNode> source, std::shared_ptr<SinkNode> sink)
        : Identifiable(id), _baseProbability(probability), _source(source), _sink(sink)
    {
        _probability = _baseProbability;
    }

    void Link::passProduct(Product::Ptr &&product) { _sink->moveInProduct(std::move(product)); }

    double Link::getProbability() const { return _probability; }

    double Link::getBaseProbability() const { return _baseProbability; }

    void Link::setProbability(double newProbability) { _probability = newProbability; }

    void SourceLinksHub::bindLink(Link::Ptr link)
    {
        _links.emplace_back(link);
        normalize();
    }

    void SourceLinksHub::unBindLink(size_t id)
    {
        std::erase_if(_links, [id](Link::Ptr link)
                      { return link->getId() == id; });
        normalize();
    }

    void SourceLinksHub::unBindLink(Link::Ptr linkToRemove)
    {
        std::erase_if(_links, [linkToRemove](Link::Ptr link)
                      { return link == linkToRemove; });
        normalize();
    }

    void SourceLinksHub::passProduct(Product::Ptr &&product)
    {
        auto link = getRandomLink();
        if (!link)
        {
            throw std::runtime_error("No links available");
        }
        link->passProduct(std::move(product));
    }

    bool SourceLinksHub::connected() const { return !_links.empty(); }

    Link::Ptr SourceLinksHub::getRandomLink() const
    {
        if (_links.empty())
        {
            return nullptr;
        }
        double propability = Random::get().nextDouble();
        double accumulate = 0;
        for (auto &link : _links)
        {
            accumulate += link->getProbability();
            if (propability <= accumulate)
            {
                return link;
            }
        }
        return _links.back();
    }

    void SourceLinksHub::normalize()
    {

        double totalPropability = 0;
        for (auto &link : _links)
        {
            totalPropability += link->getBaseProbability();
        }
        for (auto &link : _links)
        {
            auto baseProbability = link->getBaseProbability();
            link->setProbability(baseProbability / totalPropability);
        }
    }

    void SinkLinksHub::bindLink(Link::Ptr link) { _links.emplace_back(link); }

    void SinkLinksHub::unBindLink(size_t id)
    {
        std::erase_if(_links, [id](Link::WeakPtr weakLink)
                      {
                          if (auto link = weakLink.lock())
                          {
                              return link->getId() == id;
                          }
                          return true;
                      });
    }

    void SinkLinksHub::unBindLink(Link::Ptr linkToRemove)
    {
        std::erase_if(_links, [linkToRemove](Link::WeakPtr weakLink)
                      {
                          if (auto link = weakLink.lock())
                          {
                              return link == linkToRemove;
                          }
                          return true;
                      });
    }

    bool SinkLinksHub::connected() const { return !_links.empty(); }
}